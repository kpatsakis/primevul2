static int __packet_snd_vnet_parse(struct virtio_net_hdr *vnet_hdr, size_t len)
{
	unsigned short gso_type = 0;

	if ((vnet_hdr->flags & VIRTIO_NET_HDR_F_NEEDS_CSUM) &&
	    (__virtio16_to_cpu(vio_le(), vnet_hdr->csum_start) +
	     __virtio16_to_cpu(vio_le(), vnet_hdr->csum_offset) + 2 >
	      __virtio16_to_cpu(vio_le(), vnet_hdr->hdr_len)))
		vnet_hdr->hdr_len = __cpu_to_virtio16(vio_le(),
			 __virtio16_to_cpu(vio_le(), vnet_hdr->csum_start) +
			__virtio16_to_cpu(vio_le(), vnet_hdr->csum_offset) + 2);

	if (__virtio16_to_cpu(vio_le(), vnet_hdr->hdr_len) > len)
		return -EINVAL;

	if (vnet_hdr->gso_type != VIRTIO_NET_HDR_GSO_NONE) {
		switch (vnet_hdr->gso_type & ~VIRTIO_NET_HDR_GSO_ECN) {
		case VIRTIO_NET_HDR_GSO_TCPV4:
			gso_type = SKB_GSO_TCPV4;
			break;
		case VIRTIO_NET_HDR_GSO_TCPV6:
			gso_type = SKB_GSO_TCPV6;
			break;
		case VIRTIO_NET_HDR_GSO_UDP:
			gso_type = SKB_GSO_UDP;
			break;
		default:
			return -EINVAL;
		}

		if (vnet_hdr->gso_type & VIRTIO_NET_HDR_GSO_ECN)
			gso_type |= SKB_GSO_TCP_ECN;

		if (vnet_hdr->gso_size == 0)
			return -EINVAL;
	}

	vnet_hdr->gso_type = gso_type;	/* changes type, temporary storage */
	return 0;
}
